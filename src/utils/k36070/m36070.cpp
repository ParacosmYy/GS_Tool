#include "k36070/m36070.h"
QVector<double> m36070::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
