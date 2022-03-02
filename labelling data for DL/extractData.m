%% extract 48*48 image data for training and testing deep network according to the result of detectFish.m
% path_src = 'raw data/';
% filename_in = '2022_01_07-10_46_12.avi';
% num_keyframes = 243;
% shift_head2center = [11,0];
% template_size = [48,48];
sample_size = [110,110];
thresh_bin = 50.0/255;
thresh_area = 100;

prefix = [path_src, filename_in];
prefix(end - 3:end) = [];
% extension = 'png';
path_dst = [path_root,filename_in];
path_dst(end - 3:end) = [];
path_dst = [path_dst,'_original/'];
filename_dst = [path_dst, 'labels.txt'];

% extract samples
idx_sample = 1;
labels = [];
distance_thresh = abs(sample_size - template_size) / 2.0;
for i = 1:num_keyframes
    filename_keyframe = [prefix , '_' , num2str(i,'%05d') , '.' , extension];
    keyframe = imread(filename_keyframe);
    BW = imbinarize(keyframe,thresh_bin);
    CC = bwconncomp(BW);
    Centroid = regionprops(CC,'Centroid');
    Area = regionprops(CC,'Area');
    for j = 1:CC.NumObjects
        if Area(j).Area>thresh_area
            distance2fish(1) = abs(Centroid(j).Centroid(2) - result(i).center(2));
            distance2fish(2) = abs(Centroid(j).Centroid(1) - result(i).center(1));
            if  (result(i).nonfish == 0) && distance2fish(1)<distance_thresh(1) && distance2fish(2)<distance_thresh(2) % positive samples(fish)
                range_x1 = round(Centroid(j).Centroid(1) - sample_size(2)/2);
                range_x2 = round(Centroid(j).Centroid(1) + sample_size(2)/2 - 1);
                range_y1 = round(Centroid(j).Centroid(2) - sample_size(1)/2);
                range_y2 = round(Centroid(j).Centroid(2) + sample_size(1)/2 - 1);
                % index out of range
                if range_x1<1
                    range_x1 = 1;
                    range_x2 = range_x1 + sample_size(2) - 1;
                end
                if range_x2>size(keyframe,2)
                    range_x2 = size(keyframe,2);
                    range_x1 = range_x2 - sample_size(2) + 1;
                end
                if range_y1<1
                    range_y1 = 1;
                    range_y2 = range_y1 + sample_size(1) - 1;
                end
                if range_y2>size(keyframe,1)
                    range_y2 = size(keyframe,1);
                    range_y1 = range_y2 - sample_size(1) + 1;
                end
                sample = keyframe(range_y1:range_y2,range_x1:range_x2);
                filename_dst = [path_dst,num2str(idx_sample,'%05d') , '.' , extension];
                imwrite(sample,filename_dst);
                labels = [labels, 1];
                idx_sample = idx_sample + 1;
            else % negative samples(nonfish connective component)
                range_x1 = round(Centroid(j).Centroid(1) - sample_size(2)/2);
                range_x2 = round(Centroid(j).Centroid(1) + sample_size(2)/2 - 1);
                range_y1 = round(Centroid(j).Centroid(2) - sample_size(1)/2);
                range_y2 = round(Centroid(j).Centroid(2) + sample_size(1)/2 - 1);
                % index out of range
                if range_x1<1 || range_x2>size(keyframe,2) || range_y1<1 || range_y2>size(keyframe,1)
                    continue;
                end
                sample = keyframe(range_y1:range_y2,range_x1:range_x2);
                filename_dst = [path_dst,num2str(idx_sample,'%05d') , '.' , extension];
                imwrite(sample,filename_dst);
                labels = [labels, 0];
                idx_sample = idx_sample + 1;
            end
        end
    end
end

% output the labels
num_samples = idx_sample - 1;
file_labels = fopen(filename_dst,'w');
for i = 1:num_samples
    fprintf(file_labels,'%d , %d\r\n',i,labels(i));
end
fclose(file_labels);
