#include "e21704/m21704.h"
QVector<double> m21704::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
