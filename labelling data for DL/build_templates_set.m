%% load, align and average all the pictures
num_frames = 3;
head = [381, 181; 385, 179; 398, 181];
yolk = [394, 165; 404, 187; 378, 183];
shift_head2center = [13,0];
template_size = 50; % (145:194, 156:205)
num_templates = 72;
% path_src = 'raw data/';
% filename_in = '2022_01_07-10_46_12.avi';
% extension = 'png';
path_templates = 'templates/';
path_dst = [path_src,path_templates];
prefix = [path_src,filename_in];
prefix(end - 3:end) = [];
for i=1:num_frames
    filename = [prefix , '_' , num2str(i,'%05d') , '.' , extension];
    heading_vector = head(i,:) - yolk(i,:);
    heading_vector = heading_vector./norm(heading_vector);
    theta = atan2(-heading_vector(2), heading_vector(1));
    %% load one picture
    I = imread(filename);
    %% translate the center of the fish brain to the center of the image
    center_image(1) = (size(I,2) + 1)/2;
    center_image(2) = (size(I,1) + 1)/2;
    J = imtranslate(I, center_image - head(i,:), 'FillValues', 0);
    %% rotate the fish heading vector to the right
    J = imrotate(J, -theta/pi*180, 'bilinear', 'crop');
    %% display
%     figure;
%     imshow(I);
%     title(['Original Image ', num2str(i,'%02d')]);
%     figure;
%     imshow(J);
%     title(['Ratated and Translated Image ', num2str(i,'%02d')]);
    if i==1
        average = single(J);
    else
        average = average + single(J);
    end
end
average = average./num_frames;
%% augment by flipping
average_flip = flipud(average);
%% display
% figure;
% imshow(uint8(average_flip));
% title('Fliped Average Image');
average = average + average_flip;
average = average./2;
average = uint8(average);
%% display
% figure;
% imshow(average);
% title('Average Image');

%% translate, rotate and crop
prefix = [path_dst,filename_in];
prefix(end - 3:end) = [];
center_image(1) = (size(I,2) + 1)/2;
center_image(2) = (size(I,1) + 1)/2;
center_template = center_image - shift_head2center; %[169.5, 180.5];
average = imtranslate(average, center_image - center_template,'FillValues',0);
%% display
figure;
imshow(uint8(average));
title('Template 0 Uncropped');
%% save the templates
for i=0:num_templates - 1
    theta = i*360/num_templates;
    J = imrotate(average, theta, 'bilinear', 'crop');
    range_template_x = center_image(1) - (template_size - 1)/2 : center_image(1) + (template_size - 1)/2;
    range_template_y = center_image(2) - (template_size - 1)/2 : center_image(2) + (template_size - 1)/2;
    range_template_x = uint16(range_template_x);
    range_template_y = uint16(range_template_y);
    template = J(range_template_y, range_template_x);
    filename = [prefix , '_template' , num2str(i,'%03d') , '.' , extension];
    imwrite(template, filename);
end