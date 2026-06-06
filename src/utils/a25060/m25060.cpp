#include "a25060/m25060.h"
QVector<double> m25060::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
