#include "k8830/m8830.h"
QVector<double> m8830::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
