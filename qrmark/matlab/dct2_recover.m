%Name:		Chris Shoemaker
%Course:	EER-280 - Digital Watermarking
%Project: 	Threshold-Based Correlation in DCT mid-band
%           Uses two PN sequences; one for a "0" and another for a "1"
%           Watermark Recovery

clear all;

% save start time
start_time=cputime;

blocksize=8;                    % set the dct blocksize

midband=[   0,0,0,1,1,1,1,0;    % defines the mid-band frequencies of an 8x8 dct
            0,0,1,1,1,1,0,0;
            0,1,1,1,1,0,0,0;
            1,1,1,1,0,0,0,0;
            1,1,1,0,0,0,0,0;
            1,1,0,0,0,0,0,0;
            1,0,0,0,0,0,0,0;
            0,0,0,0,0,0,0,0 ];
        
% read in the watermarked object
file_name='dct2_watermarked_mod.bmp';
watermarked_image=double(imread(file_name));

% determine size of watermarked image
Mw=size(watermarked_image,1);	        %Height
Nw=size(watermarked_image,2);	        %Width

% determine maximum message size based on cover object, and blocksize
max_message=Mw*Nw/(blocksize^2);

% read in original watermark
file_name='_copyright.bmp';
orig_watermark=double(imread(file_name));

% determine size of original watermark
Mo=size(orig_watermark,1);	%Height
No=size(orig_watermark,2);	%Width

% read in key for PN generator
file_name='_key.bmp';
key=double(imread(file_name))./256;

% reset MATLAB's PN generator to state "key"
rand('state',key);

% generate PN sequence
pn_sequence_zero=round(2*(rand(1,sum(sum(midband)))-0.5));

% process the image in blocks
x=1;
y=1;
for (kk = 1:max_message)

    % transform block using DCT
    dct_block=dct2(watermarked_image(y:y+blocksize-1,x:x+blocksize-1));
    
    % extract the middle band coeffcients
    ll=1;
    for ii=1:blocksize
        for jj=1:blocksize
            if (midband(jj,ii)==1)
                sequence(ll)=dct_block(jj,ii);
                ll=ll+1;
            end
        end
    end
    
    % calculate the correlation of the middle band sequence to pn sequence
    correlation(kk)=corr2(pn_sequence_zero,sequence);

    % move on to next block. At and of row move to next row
    if (x+blocksize) >= Nw
        x=1;
        y=y+blocksize;
    else
        x=x+blocksize;
    end
end

% if correlation exceeds threshold, set bit to '0', otherwise '1'
for (kk=1:Mo*No)
    if (correlation(kk) < mean(correlation(1:Mo*No)))
        message_vector(kk)=0;
    else
        message_vector(kk)=1;
    end
end

% reshape the embeded message
message=reshape(message_vector(1:Mo*No),Mo,No);

% display processing time
elapsed_time=cputime-start_time,

% display recovered message
figure(2)
imshow(message,[])
title('Recovered Message')