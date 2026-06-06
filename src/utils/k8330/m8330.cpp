#include "k8330/m8330.h"
QVector<double> m8330::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
