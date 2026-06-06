#include "i35708/m35708.h"
QVector<double> m35708::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
