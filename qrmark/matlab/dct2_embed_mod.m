%Name:		Chris Shoemaker
%Course:	EER-280 - Digital Watermarking
%Project: 	Comparison-Based Correlation in DCT mid-band
%           Uses two PN sequences; one for a "0" and another for a "1"
%           Watermark Embeding

clear all;

% save start time
start_time=cputime;

k=15;                           % set gain factor for embeding
blocksize=8;                    % set the dct blocksize
pn_sequence_search='T';         % perform search to find highly uncorrelated pn sequences {T,F}

midband=[   0,0,0,1,1,1,1,0;    % defines the mid-band frequencies of an 8x8 dct
            0,0,1,1,1,1,0,0;
            0,1,1,1,1,0,0,0;
            1,1,1,1,0,0,0,0;
            1,1,1,0,0,0,0,0;
            1,1,0,0,0,0,0,0;
            1,0,0,0,0,0,0,0;
            0,0,0,0,0,0,0,0 ];
        
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

% pad the message out to the maximum message size with ones's
message_vector=ones(1,max_message);
message_vector(1:length(message))=message;

% generate shell of watermarked image
watermarked_image=cover_object;

% read in key for PN generator
file_name='_key.bmp';
key=double(imread(file_name))./256;

% reset MATLAB's PN generator to state "key"
rand('state',key);

% generate PN sequences for "1" and "0"
pn_sequence_one=round(2*(rand(1,sum(sum(midband)))-0.5));
pn_sequence_zero=round(2*(rand(1,sum(sum(midband)))-0.5));

% find two highly un-correlated PN sequences
if (pn_sequence_search=='T')
    while (corr2(pn_sequence_one,pn_sequence_zero) > -0.55)
        pn_sequence_one=round(2*(rand(1,sum(sum(midband)))-0.5));
        pn_sequence_zero=round(2*(rand(1,sum(sum(midband)))-0.5));
    end
end
    
% process the image in blocks
x=1;
y=1;
for (kk = 1:length(message_vector))

    % transform block using DCT
    dct_block=dct2(cover_object(y:y+blocksize-1,x:x+blocksize-1));
    
    % if message bit contains zero then embed pn_sequence_zero into the mid-band
    % componants of the dct_block
    ll=1;
    if (message_vector(kk)==0)
        for ii=1:blocksize
            for jj=1:blocksize
                if (midband(jj,ii)==1)
                    dct_block(jj,ii)=dct_block(jj,ii)+k*pn_sequence_zero(ll);
                    ll=ll+1;
                end
            end
        end
    
    % otherwise, embed pn_sequence_one into the mid-band componants of dct_block    
    else
        for ii=1:blocksize
            for jj=1:blocksize
                if (midband(jj,ii)==1)
                    dct_block(jj,ii)=dct_block(jj,ii)+k*pn_sequence_one(ll);
                    ll=ll+1;
                end
            end
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
imwrite(watermarked_image_int,'dct2_watermarked_mod.bmp','bmp');

% display processing time
elapsed_time=cputime-start_time,

% display psnr of watermarked image
psnr=psnr(cover_object,watermarked_image,Nc,Mc),

% display watermarked image
figure(1)
imshow(watermarked_image,[])
title('Watermarked Image')
