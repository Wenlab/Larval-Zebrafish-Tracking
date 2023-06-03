import torch
import numpy as np
import onnx
import onnxruntime as ort

import simpleModel as trackModel

modelName='trackKeyPointModel_0618_unet_320crop'

device=torch.device("cuda:3" if torch.cuda.is_available() else "cpu")
# device=torch.device("cpu")

modelPath='./model/'+modelName+'.pt'
model=trackModel.load_network(device, path=modelPath)
model=model.to(device)


x = torch.randn(1, 1, 320, 320, requires_grad=True).to(device)
torch_out = model(x)

# Export the model
torch.onnx.export(model,               # model being run
                  x,                         # model input (or a tuple for multiple inputs)
                  modelName + '.onnx',   # where to save the model (can be a file or file-like object)
                  export_params=True,        # store the trained parameter weights inside the model file
                  opset_version=9,          # the ONNX version to export the model to
                  verbose=True,
                  do_constant_folding=True,  # whether to execute constant folding for optimization
                  input_names = ['input'],   # the model's input names
                  output_names = ['output'], # the model's output names
                #   dynamic_axes={'input' : {0 : 'batch_size'},    # variable lenght axes
                #                 'output' : {0 : 'batch_size'}}
                                )

onnxmodel = onnx.load(modelName + '.onnx')
onnx.checker.check_model(onnxmodel)


import onnxruntime
ort_session = onnxruntime.InferenceSession(modelName + '.onnx',providers=['CUDAExecutionProvider'])
# compute ONNX Runtime output prediction
outputs = ort_session.run(None,{ 'input' : x.cpu().detach().numpy()}) 
# compare ONNX Runtime and PyTorch results
np.testing.assert_allclose(torch_out.cpu().detach().numpy(), outputs[0], rtol=1e-03, atol=1e-05)
print("Exported model has been tested with ONNXRuntime, and the result looks good!")