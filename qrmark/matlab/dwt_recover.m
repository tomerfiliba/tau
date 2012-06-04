%Name:		Chris Shoemaker
%Course:	EER-280 - Digital Watermarking
%Project: 	Embeding of CDMA watermark into H1,V1,D1 componants of a 1-scale DWT
%           Watermark Recovery

clear all;

% save start time
start_time=cputime;

% read in the watermarked object
file_name='dwt_watermarked.bmp';
watermarked_image=double(imread(file_name));

% determine size of watermarked image
Mw=size(watermarked_image,1);	        %Height
Nw=size(watermarked_image,2);	        %Width

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

% initalize message to all ones
message_vector=ones(1,Mo*No);

[cA1,cH1,cV1,cD1] = dwt2(watermarked_image,'haar');

% add pn sequences to H1 and V1 componants when message = 0 
for (kk=1:length(message_vector))
    pn_sequence_h=round(2*(rand(Mw/2,Nw/2)-0.5));
    pn_sequence_v=round(2*(rand(Mw/2,Nw/2)-0.5));
    
    correlation_h(kk)=corr2(cH1,pn_sequence_h);
    correlation_v(kk)=corr2(cV1,pn_sequence_v);
    correlation(kk)=(correlation_h(kk)+correlation_v(kk))/2;
end


for (kk=1:length(message_vector))
    if (correlation(kk) > mean(correlation))
        message_vector(kk)=0;
    end
end

% reshape the message vector and display recovered watermark.
figure(2)
message=reshape(message_vector,Mo,No);
imshow(message,[])
title('Recovered Watermark')

% display processing time
elapsed_time=cputime-start_time,
