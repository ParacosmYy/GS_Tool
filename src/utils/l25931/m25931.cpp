#include "l25931/m25931.h"
QVector<double> m25931::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
