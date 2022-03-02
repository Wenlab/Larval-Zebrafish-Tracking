% load and extract key frames
path_root = 'D:/Daguang Li/tracking by deep learning/labelling data/';
path_src = [path_root,'raw data/'];
filename_in = '2022_01_07-10_46_12.avi';
step_size = 500;
KeyFrames = extractKeyFrames([path_src,filename_in],step_size);
num_keyframes = size(KeyFrames,4);
% save the key frames
prefix = [path_src,filename_in];
prefix(end - 3:end) = [];
extension = 'png';
for i = 1:num_keyframes
    filename_keyframe = [prefix , '_' , num2str(i,'%05d') , '.' , extension];
    imwrite(rgb2gray(KeyFrames(:,:,:,i)),filename_keyframe);
end

% build templates set
%% manually label the head and yolk of fish for several frames in build_templates_set.m
build_templates_set;

% label the head and heading vector of fish for several frames
result = detectFish(path_src,filename_in,num_keyframes,[path_src,path_templates],num_templates,shift_head2center);
% path_templates, num_templates, shift_head2center are set in build_templates_set.m

%% manually label the frame without fish
% for i = 1:num_keyframes
%     result(i).nonfish = 0;
% end
% result(131).nonfish = 1;

%% manually set the length from head to yolk
shift_head2yolk = 22;
% calculate the position of yolk
for i = 1:num_keyframes
    result(i).yolk = result(i).head - shift_head2yolk*result(i).heading_vector;
end
%% remember to save the variable 'result'!!!
% display
figure;
prefix = [path_src,filename_in];
prefix(end - 3:end) = [];
for i = 1:num_keyframes
    % imshow(KeyFrames(:,:,:,i));
    filename_keyframe = [prefix , '_' , num2str(i,'%05d') , '.' , extension];
    keyframe = imread(filename_keyframe);
    imshow(keyframe);
    if result(i).nonfish==0
        pos = [result(i).head - 2, 5, 5];
        rectangle('Position',pos,'Curvature',[1 1],'EdgeColor','r');
        pos = [result(i).yolk - 1, 3, 3];
        rectangle('Position',pos,'Curvature',[1 1],'EdgeColor','r');
    else
        text(180,180,'no fish');
    end
    title(num2str(i));
    waitforbuttonpress;
end

% % extract samples
% extractData;
% %% check manually
% extract_augment_Data;

augmentData