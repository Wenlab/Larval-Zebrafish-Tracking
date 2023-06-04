%% 1.load video and extract key frames
% it takes long time
path_root = 'F:\Kexin\dl-tracking\newFishVideo\raw data\';
path_video=[path_root,'video\'];

path_allData=[path_root,'labelImage\'];
if(~exist(path_root))
    mkdir(path_root);
end

allvideoName=dir(fullfile(path_video,'*.avi'));

for i=1:length(allvideoName)
    videoName = allvideoName(i).name;
    a = find('.'==videoName);
    videoName_noExt=videoName(1:a-1);
    path_videoDataRoot=[path_allData,videoName_noExt,'\'];
    if(~exist(path_videoDataRoot))
        mkdir(path_videoDataRoot);
    end

    step_size = 500;
    KeyFrames = extractKeyFrames([path_video,videoName],step_size);
    num_keyframes = size(KeyFrames,4);
    if(num_keyframes<3)
        continue;
    end

    % save the key frames
    path_videoDataRoot_rawImg = [path_videoDataRoot,'raw_data/'];
    if(~exist(path_videoDataRoot_rawImg))
        mkdir(path_videoDataRoot_rawImg);
    end
    
    extension = 'png';
    disp(['save ',videoName,' KeyFrames...'])
    for j = 1:num_keyframes
        filename_keyframe = [path_videoDataRoot_rawImg , '/' ,videoName_noExt,'_' ,num2str(j,'%05d') , '.' , extension];
        imwrite(rgb2gray(KeyFrames(:,:,:,j)),filename_keyframe);
    end
end

% build templates set
%% 2.manually label the head and yolk of fish for several frames in build_templates_set.m
videoFolders=getsubfolders(path_allData);
for i=1:length(videoFolders)
    path_videoDataRoot=[path_allData,'/',videoFolders{i}];
    build_templates_set(path_videoDataRoot);
end

%% 3.detect fish head and yolk on keyFrames
% takes long time
for j=1:length(videoFolders)
    % label the head and heading vector of fish for several frames
    % load params from file
    path_videoDataRoot=[path_allData,'/',videoFolders{j}];
    path_videoDataRoot_rawImg = [path_videoDataRoot,'/raw_data/'];
    path_videoDataRoot_template=[path_videoDataRoot,'/template/'];
    try
    load([path_videoDataRoot,'/','template_params.mat']);
    catch ME
        disp(ME.message)
        continue
    end
    disp(['detection Fish....: ',path_videoDataRoot_rawImg])
    tic
    result = detectFish(path_videoDataRoot_rawImg,path_videoDataRoot_template,shift_head2center);
    toc
    % path_templates, num_templates, shift_head2center are set in build_templates_set.m
    if(length(result)<10)   
        continue
    end
    
    % manually set the length from head to yolk
    shift_head2yolk = 21;
    % calculate the position of yolk
    allkeyFrameName=dir(fullfile(path_videoDataRoot_rawImg,'*.png'));
    for i = 1:length(allkeyFrameName)
        result(i).yolk = result(i).head - shift_head2yolk*result(i).heading_vector;
    end
    save([path_videoDataRoot,'/','result_raw.mat'],'result')
end



%% 4.display and check
for aa=1:length(videoFolders)
    path_videoDataRoot=[path_allData,'/',videoFolders{aa}];
    path_videoDataRoot_keyFrames=[path_videoDataRoot,'/','raw_data'];
    allkeyFrameName=dir(fullfile(path_videoDataRoot_keyFrames,'*.png'));
    try
        load([path_videoDataRoot,'/','result_raw.mat']);
    catch
        disp(['cannot load ',path_videoDataRoot,'/','result_raw.mat'])
        continue
    end
    disp(['check ',path_videoDataRoot,' key image results'])
    
    i=1;
    while i<length(allkeyFrameName)
        % imshow(KeyFrames(:,:,:,i));
        filename_keyframe = [path_videoDataRoot_keyFrames,'/',allkeyFrameName(i).name];
        keyframe = imread(filename_keyframe);
        imshow(keyframe);
        pos = [result(i).head - 2, 5, 5];
        rectangle('Position',pos,'Curvature',[1 1],'EdgeColor','r');
        pos = [result(i).yolk - 1, 3, 3];
        rectangle('Position',pos,'Curvature',[1 1],'EdgeColor','g');
        title([num2str(i),'/',num2str(length(allkeyFrameName))]);
        result(i).nonfish = 0;
        pause;
        if 'a'==get(gcf,'CurrentCharacter')
            result(i).nonfish = 1;
        end 
        if 'd'==get(gcf,'CurrentCharacter')
            result(i).nonfish = 1;
            figure;
            set(gcf,'outerposition',get(0,'screensize'));
            imagesc(keyframe);
            axis equal
            title('point head');   
            [xx,yy] = ginput(1);
            hold on;plot(xx,yy,'ro');
            close all;
            head(m,1)=xx;
            head(m,2)=yy;

            figure;
            set(gcf,'outerposition',get(0,'screensize'));
            imagesc(keyframe);
            axis equal
            title('point yolk');   
            [xx,yy] = ginput(1);
            hold on;plot(xx,yy,'ro');
            close all;
            yolk(m,1)=xx;
            yolk(m,2)=yy;
            
            result(i).head = head;
            result(i).yolk = yolk;
        end 
        if('s'==get(gcf,'CurrentCharacter'))  %退回到前一帧
            i=i-2;
        end
        i=i+1;
    end
    % save the variable 'result'
    filename_dst = [path_videoDataRoot,'/result_check.mat'];
    save(filename_dst,'result');
    close all
end



%% 5.augmentData
for i=1:length(videoFolders)
    path_videoDataRoot=[path_allData,'/',videoFolders{i}];
    try
        load([path_videoDataRoot,'/','result_check.mat']);
        disp(['load ',path_videoDataRoot,'/','result_check.mat'])
    catch
        disp(['cannot load ',path_videoDataRoot,'/','result_check.mat'])
        continue
    end
    augmentData(path_videoDataRoot,result);

end

%% some codes for format transformation, padding etc.
% path_root = 'D:/Daguang Li/tracking by deep learning/labelling data/';
% path_src = [path_root,'raw data/'];
% filename_in = 'complex_background_maze.mov';
% v = VideoReader([path_src,filename_in]);
% filename_out = filename_in;
% filename_out(end-3:end) = [];
% filename_out = [filename_out,'.avi'];
% writer = VideoWriter([path_src,filename_out],'Uncompressed AVI');
% writer.FrameRate = v.FrameRate;
% open(writer);
% while hasFrame(v)
% img = readFrame(v);
% writeVideo(writer,img);
% end
% close(writer);

% for i = 1:num_keyframes
%     C = KeyFrames(:,:,:,i);
%     C = rgb2gray(C);
%     D = padarray(C,[44 80],0,'both');
%     filename_keyframe = [prefix , '_' , num2str(i,'%05d') , '.' , extension];
%     imwrite(D,filename_keyframe);
% end