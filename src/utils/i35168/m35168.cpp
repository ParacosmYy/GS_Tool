#include "i35168/m35168.h"
QVector<double> m35168::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
