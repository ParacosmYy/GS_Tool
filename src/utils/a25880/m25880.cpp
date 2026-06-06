#include "a25880/m25880.h"
QVector<double> m25880::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
