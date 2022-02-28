function KeyFrames = extractKeyFrames(filename,step_size)
%% Read from the video of filename, extract key frames by k-means method, assuming that statistically there is one key frame in every step_size frames.
%% step_size: swim bout interval(frame), can be estimated by the frequency of swim bouts

    %% v = VideoReader('2021_12_29-15_24_16.avi');
    v = VideoReader(filename);
    num_frames = v.NumFrames;
    %num_keyframes = floor(num_frames / step_size);
    chunk_size = 5000;
    num_chunks = ceil(num_frames/chunk_size);

    idx_keyframe = 1;
    for idx_chunk = 1:num_chunks
        idx_frame_start = (idx_chunk - 1)*chunk_size + 1;
        idx_frame_end = (idx_chunk - 1)*chunk_size + chunk_size;
        if idx_frame_end>num_frames
            idx_frame_end = num_frames;
        end
        num_frames_chunk = idx_frame_end - idx_frame_start + 1;
        num_keyframes_chunk = floor(num_frames_chunk / step_size);
        if num_keyframes_chunk<1
            num_keyframes_chunk = 1;
        end
        frames = read(v,[idx_frame_start, idx_frame_end]);
        frames = mean(frames,3);
        frames = single(frames);
        frames = reshape(frames,[],num_frames_chunk);
        frames = frames';
        idx = kmeans(frames,num_keyframes_chunk*3);
        for i = 1:num_keyframes_chunk*3
            index = find(idx==i,1);
            index = index + (idx_chunk - 1)*chunk_size;
            KeyFrames(:,:,:,idx_keyframe) = read(v,index);
            idx_keyframe = idx_keyframe + 1;
        end
    end
    
end