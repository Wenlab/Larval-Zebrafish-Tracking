%% crop the image from 672*360 to 360*360
%% and augment data by rotation
sample_size = [360,360];
border = 20; % if the fish is outside after rotation, we will translate the image to make sure the fish is in and 'border' away from the border
prefix = [path_src, filename_in];
prefix(end - 3:end) = [];
path_dst = [path_root,filename_in];
path_dst(end - 3:end) = [];
path_dst = [path_dst,'_augmented/'];
mkdir(path_dst);

idx_sample = 1;
labels = []; % [head_x, head_y, yolk_x, yolk_y, nonfish]
for i = 1:num_keyframes
    filename_keyframe = [prefix , '_' , num2str(i,'%05d') , '.' , extension];
    keyframe = imread(filename_keyframe);
    center_image(1) = (size(keyframe,2) + 1)/2;
    center_image(2) = (size(keyframe,1) + 1)/2;
    range_x1 = round(center_image(1) - sample_size(2)/2);
    range_x2 = round(center_image(1) + sample_size(2)/2 - 1);
    range_y1 = round(center_image(2) - sample_size(1)/2);
    range_y2 = round(center_image(2) + sample_size(1)/2 - 1);
    theta_head = atan2(center_image(2)-result(i).head(2), result(i).head(1)-center_image(1));
    r_head = norm([center_image(2)-result(i).head(2), result(i).head(1)-center_image(1)]);
    theta_yolk = atan2(center_image(2)-result(i).yolk(2), result(i).yolk(1)-center_image(1));
    r_yolk = norm([center_image(2)-result(i).yolk(2), result(i).yolk(1)-center_image(1)]);
    for k = 0:10:359
        shift_image = [0,0];
        head_rotated = [r_head*cos(theta_head + k/180*pi), -r_head*sin(theta_head + k/180*pi)] + center_image;
        yolk_rotated = [r_yolk*cos(theta_yolk + k/180*pi), -r_yolk*sin(theta_yolk + k/180*pi)] + center_image;
        %
        if head_rotated(1)<range_x1
            shift_image(1) = range_x1 - head_rotated(1) + border;
        elseif head_rotated(1)>range_x2
            shift_image(1) = range_x2 - head_rotated(1) - border;
        end
        if head_rotated(2)<range_y1
            shift_image(2) = range_y1 - head_rotated(2) + border;
        elseif head_rotated(2)>range_y2
            shift_image(2) = range_y2 - head_rotated(2) - border;
        end
        %
        if yolk_rotated(1)<range_x1
            temp = range_x1 - yolk_rotated(1) + border;
            shift_image(1) = max([temp,shift_image(1)]);
        elseif yolk_rotated(1)>range_x2
            temp = range_x2 - yolk_rotated(1) - border;
            shift_image(1) = min([temp,shift_image(1)]);
        end
        if yolk_rotated(2)<range_y1
            temp = range_y1 - yolk_rotated(2) + border;
            shift_image(2) = max([temp,shift_image(2)]);
        elseif yolk_rotated(2)>range_y2
            temp = range_y2 - yolk_rotated(2) - border;
            shift_image(2) = min([temp,shift_image(2)]);
        end
        J = imrotate(keyframe, k, 'bilinear');
        center_J(1) = (size(J,2) + 1)/2;
        center_J(2) = (size(J,1) + 1)/2;
        range_x1_new = round(center_J(1) - shift_image(1) - sample_size(2)/2);
        range_x2_new = round(center_J(1) - shift_image(1) + sample_size(2)/2 - 1);
        range_y1_new = round(center_J(2) - shift_image(2) - sample_size(1)/2);
        range_y2_new = round(center_J(2) - shift_image(2) + sample_size(1)/2 - 1);
        sample = J(range_y1_new:range_y2_new,range_x1_new:range_x2_new);
        filename_dst = [path_dst,num2str(idx_sample,'%05d') , '.' , extension];
        imwrite(sample,filename_dst);
        labels = [labels; head_rotated + shift_image + [1 - range_x1, 1 - range_y1], yolk_rotated + shift_image + [1 - range_x1, 1 - range_y1], result(i).nonfish];
        idx_sample = idx_sample + 1;

        % % display
        % imshow(sample);
        % if labels(idx_sample-1,5)==0
        %     pos = [labels(idx_sample-1, 1:2) - 2, 5, 5];
        %     rectangle('Position',pos,'Curvature',[1 1],'EdgeColor','r');
        %     pos = [labels(idx_sample-1, 3:4) - 1, 3, 3];
        %     rectangle('Position',pos,'Curvature',[1 1],'EdgeColor','r');
        % else
        %     text(180,180,'no fish');
        % end
        % title(num2str(idx_sample-1));
        % waitforbuttonpress;
    end
end
% output the labels
filename_dst = [path_dst, 'labels.txt'];
num_samples = idx_sample - 1;
file_labels = fopen(filename_dst,'w');
for i = 1:num_samples
    fprintf(file_labels,'%d , %.2f , %.2f , %.2f , %.2f , %d\r\n',i,labels(i,1),labels(i,2),labels(i,3),labels(i,4),labels(i,5));
end
fclose(file_labels);