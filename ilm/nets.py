import itertools
from pybrain.structure import RecurrentNetwork
from pybrain.supervised.trainers import BackpropTrainer
from pybrain.datasets import SupervisedDataSet

john=1
bill=2
sue=3
mary=4
love=10
see=11

ds = SupervisedDataSet(2, 1)

for verb in [love, see]: 
    for a, b in itertools.combinations([john, bill, sue, mary]):
        ds.addSample((verb,a,b), (1,))

n = RecurrentNetwork()
n.addInputModule(LinearLayer(3, name='in'))
n.addModule(SigmoidLayer(3, name='hidden'))
n.addOutputModule(LinearLayer(1, name='out'))
n.addConnection(FullConnection(n['in'], n['hidden'], name='c1'))
n.addConnection(FullConnection(n['hidden'], n['out'], name='c2'))
n.addRecurrentConnection(FullConnection(n['hidden'], n['hidden'], name='c3'))

trainer = BackpropTrainer(n, ds)
trainer.train()


