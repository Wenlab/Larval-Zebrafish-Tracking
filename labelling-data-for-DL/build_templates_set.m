%% load, align and average all the pictures
function build_templates_set(path)
    path_template=[path,'/template/'];
    if(~exist(path_template))
        mkdir(path_template);
    end
    path_rawImg=[path,'/raw_data/'];
    extension = 'png';
    
    allfileName=dir(fullfile(path_rawImg,'*.png'));
    if(length(allfileName)<3)
        disp('cannot find enough Images')
        return
    end
    num_frames = 3;
    head=zeros(num_frames,2);
    yolk=zeros(num_frames,2);
    for m=1:num_frames
        image=imread([path_rawImg,'\',allfileName(m).name]);
        figure;
        set(gcf,'outerposition',get(0,'screensize'));
        imagesc(image);
        axis equal
        title('point head');   
        [xx,yy] = ginput(1);
        hold on;plot(xx,yy,'ro');
        close all;
        head(m,1)=xx;
        head(m,2)=yy;

        figure;
        set(gcf,'outerposition',get(0,'screensize'));
        imagesc(image);
        axis equal
        title('point yolk');   
        [xx,yy] = ginput(1);
        hold on;plot(xx,yy,'ro');
        close all;
        yolk(m,1)=xx;
        yolk(m,2)=yy;
    end

    shift_head2center = [12,0];
    template_size = 50; % (145:194, 156:205)
    num_templates = 72;
    save([path,'/template_params.mat'],'num_templates','shift_head2center');
    % path_src = 'raw data/';
    % filename_in = '2022_01_07-10_46_12.avi';
    % extension = 'png';
    % prefix = [path_src,filename_in];
    % prefix(end - 3:end) = [];

    for i=1:num_frames
        filename = [path_rawImg,'\',allfileName(i).name];
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
    center_image(1) = (size(I,2) + 1)/2;
    center_image(2) = (size(I,1) + 1)/2;
    center_template = center_image - shift_head2center; %[169.5, 180.5];
    average = imtranslate(average, center_image - center_template,'FillValues',0);
    %% display
    figure;
    imshow(uint8(average));
    title('Template 0 Uncropped');
    %% save the templates
    disp(['template savepath: ', path_template]);
    for i=0:num_templates - 1
        theta = i*360/num_templates;
        J = imrotate(average, theta, 'bilinear', 'crop');
        range_template_x = center_image(1) - (template_size - 1)/2 : center_image(1) + (template_size - 1)/2;
        range_template_y = center_image(2) - (template_size - 1)/2 : center_image(2) + (template_size - 1)/2;
        range_template_x = uint16(range_template_x);
        range_template_y = uint16(range_template_y);
        template = J(range_template_y, range_template_x);
        filename = [path_template , 'template' , num2str(i,'%03d') , '.' , extension];
        imwrite(template, filename);
    end
end