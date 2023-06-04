%% load, align and average all the pictures
path_source = 'D:\make templates\';
extension = '.png';
num_frames = 12;
center = [179, 172; 179, 169; 180, 172; 180, 172; 178, 171; 181, 171; 179, 172; 180, 170; 181, 171; 179, 170; 212, 169; 212, 170];
yolk = [184, 149; 199, 179; 193, 153; 167, 153; 196, 158; 171, 151; 163, 157; 166, 187; 171, 190; 190, 190; 236, 168; 211, 144];
for i=1:num_frames
    filename = [path_source, num2str(i,'%02d'), extension];
    heading_vector = center(i,:) - yolk(i,:);
    heading_vector = heading_vector./norm(heading_vector);
    theta = atan2(-heading_vector(2), heading_vector(1));
    %% load one picture
    I = imread(filename);
    %% translate the center of the fish brain to the center of the image
    center_image = (size(I) + 1)/2;
    J = imtranslate(I, center_image - center(i,:), 'FillValues', 0);
    %% rotate the fish heading vector to the right
    J = imrotate(J, -theta/pi*180, 'bilinear', 'crop');
    %% display
    % figure;
    % imshow(I);
    % title(['Original Image ', num2str(i,'%02d')]);
    % figure;
    % imshow(J);
    % title(['Ratated and Translated Image ', num2str(i,'%02d')]);
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
path_dst = 'D:\templates\';
extension = '.jpg';
center_image = (size(average) + 1)/2;
center_template = [169.5, 180.5];
size_template = 48; % (145:194, 156:205)
num_templates = 72;
average = imtranslate(average, (size(average) + 1)/2 - center_template,'FillValues',0);
%% display
figure;
imshow(uint8(average));
title('Template 0 Uncropped');
for i=0:num_templates - 1
    theta = i*360/num_templates;
    J = imrotate(average, theta, 'bilinear', 'crop');
    range_template_x = center_image(1) - (size_template - 1)/2 : center_image(1) + (size_template - 1)/2;
    range_template_y = center_image(2) - (size_template - 1)/2 : center_image(2) + (size_template - 1)/2;
    range_template_x = uint16(range_template_x);
    range_template_y = uint16(range_template_y);
    template = J(range_template_x, range_template_y);
    imwrite(template, [path_dst, int2str(i), extension]);
end