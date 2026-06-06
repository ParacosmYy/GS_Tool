#include "f28645/m28645.h"
QVector<double> m28645::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
