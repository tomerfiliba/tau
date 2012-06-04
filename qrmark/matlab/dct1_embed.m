%Name:		Chris Shoemaker
%Course:	EER-280 - Digital Watermarking
%Project: 	Block DCT Based method, using comparision between mid-band coeffcients
%           Watermark Embeding

clear all;

% save start time
start_time=cputime;

k=50;           % set minimum coeff difference
blocksize=8;    % set the size of the block in cover to be used for each bit in watermark

% read in the cover object
file_name='_lena_std_bw.bmp';
cover_object=double(imread(file_name));

% determine size of cover image
Mc=size(cover_object,1);	        %Height
Nc=size(cover_object,2);	        %Width

% determine maximum message size based on cover object, and blocksize
max_message=Mc*Nc/(blocksize^2);

% read in the message image
file_name='_copyright.bmp';
message=double(imread(file_name));
Mm=size(message,1);	                %Height
Nm=size(message,2);	                %Width

% reshape the message to a vector
message=round(reshape(message,Mm*Nm,1)./256);

% check that the message isn't too large for cover
if (length(message) > max_message)
    error('Message too large to fit in Cover Object')
end

% pad the message out to the maximum message size with ones
message_pad=ones(1,max_message);
message_pad(1:length(message))=message;

% generate shell of watermarked image
watermarked_image=cover_object;

% process the image in blocks
% encodes such that (5,2) > (4,3) when message(kk)=0
% and that (5,2) < (4,3) when message(kk)=1
x=1;
y=1;
for (kk = 1:length(message_pad))

    % transform block using DCT
    dct_block=dct2(cover_object(y:y+blocksize-1,x:x+blocksize-1));
    
    % if message bit is black, (5,2) > (4,3)
    if (message_pad(kk) == 0)

        % if (5,2) < (4,3) then we need to swap them
        if (dct_block(5,2) < dct_block(4,3))
                temp=dct_block(4,3);
                dct_block(4,3)=dct_block(5,2);
                dct_block(5,2)=temp;
        end
    
    % if message bit is white, (5,2) < (4,3)
    elseif (message_pad(kk) == 1)
        
        % if (5,2) > (4,3) then we need to swap them
        if (dct_block(5,2) >= dct_block(4,3))
                temp=dct_block(4,3);
                dct_block(4,3)=dct_block(5,2);
                dct_block(5,2)=temp;
        end
    end
    
    % now we adjust the two values such that their difference >= k
    if dct_block(5,2) > dct_block(4,3)
        if dct_block(5,2) - dct_block(4,3) < k
            dct_block(5,2)=dct_block(5,2)+(k/2);
            dct_block(4,3)=dct_block(4,3)-(k/2);            
        end
    else  
         if dct_block(4,3) - dct_block(5,2) < k
            dct_block(4,3)=dct_block(4,3)+(k/2);  
            dct_block(5,2)=dct_block(5,2)-(k/2);
        end
    end
        
    % transform block back into spatial domain
    watermarked_image(y:y+blocksize-1,x:x+blocksize-1)=idct2(dct_block);    
    
    % move on to next block. At and of row move to next row
    if (x+blocksize) >= Nc
        x=1;
        y=y+blocksize;
    else
        x=x+blocksize;
    end
end

% convert to uint8 and write the watermarked image out to a file
watermarked_image_int=uint8(watermarked_image);
imwrite(watermarked_image_int,'dct1_watermarked.bmp','bmp');

% display processing time
elapsed_time=cputime-start_time,

% display psnr of watermarked image
psnr=psnr(cover_object,watermarked_image,Nc,Mc),

% display watermarked image
figure(1)
imshow(watermarked_image,[])
title('Watermarked Image')
