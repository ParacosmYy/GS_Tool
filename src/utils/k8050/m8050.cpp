#include "k8050/m8050.h"
QVector<double> m8050::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
