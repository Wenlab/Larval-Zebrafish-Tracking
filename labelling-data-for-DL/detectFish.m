function result = detectFish(path_keyFrames,path_templates,shift_head2center)
    %% path_src: the path of source images
    %% filename: filename of the video where we extract key frames
    %% num_keyframes: number of key frames
    %% path_templates: the path from which we load templates
    %% num_templates: number of templates
    %% shift_head2center: a vector from the center of the template to the head of fish

    % load templates
    alltemplateName=dir(fullfile(path_templates,'*.png'));
    vec_templates = [];
    num_templates=length(alltemplateName);
    norm_templates = zeros(length(alltemplateName),1);
    if(num_templates<=3)
        disp('cannot find enough template')
        result=[];
        return 
    end
    
    for i = 1:num_templates
        filename_template = [path_templates,alltemplateName(i).name];
        template = imread(filename_template);
        vec_templates(:,i) = double(template(:));
        norm_templates(i) = norm(vec_templates(:,i));
    end
    [m0,n0] = size(template);
    
    % template matching    
    allkeyImgName=dir(fullfile(path_keyFrames,'*.png'));
    num_keyframes=length(allkeyImgName);
    disp(['find ',num2str(num_keyframes),' keyFrames....'])
    for k=1:num_keyframes
        result(k).center = [0,0];
        result(k).heading_vector = [0,0];
        result(k).head = [0,0] + result(k).heading_vector*norm(shift_head2center);
    end
    
    parfor k = 1:num_keyframes
        tic
        filename_keyframe = [path_keyFrames,allkeyImgName(k).name];
        keyframe = imread(filename_keyframe);
        [m,n] = size(keyframe);
        result_k_l = zeros(m-m0+1,n-n0+1,num_keyframes);
        for i = 1:m-m0+1
            for j = 1:n-n0+1
                subKeyFrame = keyframe(i:i+m0-1,j:j+m0-1);
                vec_subKeyFrame = double(subKeyFrame(:));
                for l = 1:num_templates
                    result_k_l(i,j,l) = vec_subKeyFrame'*vec_templates(:,l)/(norm(vec_subKeyFrame)*norm_templates(l));
                end
            end
        end
        [~,idx] = max(result_k_l,[],'all','linear');
        [y,x,idx_theta] = ind2sub([m-m0+1,n-n0+1,num_templates],idx);
        y = y + m0/2;
        x = x + n0/2;
        theta = 2*pi/num_templates*(idx_theta-1);
        result(k).center = [x,y];
        result(k).heading_vector = [cos(theta),-sin(theta)];
        result(k).head = [x,y] + result(k).heading_vector*norm(shift_head2center);
        disp([num2str(k),'/',num2str(num_keyframes)])
        toc
    end
end