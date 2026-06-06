#include "m35132/m35132.h"
QVector<double> m35132::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
