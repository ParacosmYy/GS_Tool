#include "k8250/m8250.h"
QVector<double> m8250::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
