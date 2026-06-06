#include "m37052/m37052.h"
QVector<double> m37052::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
