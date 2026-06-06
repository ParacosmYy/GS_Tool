#include "k8210/m8210.h"
QVector<double> m8210::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
