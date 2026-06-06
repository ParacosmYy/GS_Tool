#include "k8390/m8390.h"
QVector<double> m8390::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
