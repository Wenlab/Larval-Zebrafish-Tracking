function result = detectFish(path_src,filename,num_keyframes,path_templates,num_templates,shift_head2center)
    %% path_src: the path of source images
    %% filename: filename of the video where we extract key frames
    %% num_keyframes: number of key frames
    %% path_templates: the path from which we load templates
    %% num_templates: number of templates
    %% shift_head2center: a vector from the center of the template to the head of fish

    % load templates
    prefix = [path_templates, filename];
    prefix(end - 3:end) = [];
    extension = 'png';
    vec_templates = [];
    norm_templates = zeros(num_templates,1);
    for i = 1:num_templates
        filename_template = [prefix , '_template' , num2str(i-1,'%03d') , '.' , extension];
        template = imread(filename_template);
        vec_templates(:,i) = double(template(:));
        norm_templates(i) = norm(vec_templates(:,i));
    end
    [m0,n0] = size(template);

    % template matching
    prefix = [path_src, filename];
    prefix(end - 3:end) = [];
    extension = 'png';
    for k = 1:num_keyframes
        filename_keyframe = [prefix , '_' , num2str(k,'%05d') , '.' , extension];
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
    end
end