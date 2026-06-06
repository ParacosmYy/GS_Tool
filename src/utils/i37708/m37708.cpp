#include "i37708/m37708.h"
QVector<double> m37708::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
