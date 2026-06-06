#include "m9092/m9092.h"
QVector<double> m9092::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
