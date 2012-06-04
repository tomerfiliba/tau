%Name:		Chris Shoemaker
%Course:	EER-280 - Digital Watermarking
%Project: 	Embeding of CDMA watermark into H1,V1,D1 componants of a 1-scale DWT
%           Watermark Embeding

clear all;

% save start time
start_time=cputime;

k=2;            % set the gain factor for embeding

% read in the cover object
file_name='_lena_std_bw.bmp';
cover_object=double(imread(file_name));

% determine size of watermarked image
Mc=size(cover_object,1);	%Height
Nc=size(cover_object,2);	%Width

% read in the message image and reshape it into a vector
file_name='_copyright.bmp';
message=double(imread(file_name));
Mm=size(message,1);	                        %Height
Nm=size(message,2);	                        %Width
message_vector=round(reshape(message,Mm*Nm,1)./256);

% read in key for PN generator
file_name='_key.bmp';
key=double(imread(file_name))./256;

% reset MATLAB's PN generator to state "key"
rand('state',key);

[cA1,cH1,cV1,cD1] = dwt2(cover_object,'haar');

% add pn sequences to H1 and V1 componants when message = 0 
for (kk=1:length(message_vector))
    pn_sequence_h=round(2*(rand(Mc/2,Nc/2)-0.5));
    pn_sequence_v=round(2*(rand(Mc/2,Nc/2)-0.5));
    
    if (message(kk) == 0)
        cH1=cH1+k*pn_sequence_h;
        cV1=cV1+k*pn_sequence_v;
    end
end

% perform IDWT
watermarked_image = idwt2(cA1,cH1,cV1,cD1,'haar',[Mc,Nc]); 

% convert back to uint8
watermarked_image_uint8=uint8(watermarked_image);

% write watermarked Image to file
imwrite(watermarked_image_uint8,'dwt_watermarked.bmp','bmp');

% display processing time
elapsed_time=cputime-start_time,

% calculate the PSNR
psnr=psnr(cover_object,watermarked_image_uint8,Mc,Nc),

% display watermarked image
figure(1)
imshow(watermarked_image_uint8,[])
title('Watermarked Image')

