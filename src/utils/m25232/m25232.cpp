#include "m25232/m25232.h"
QVector<double> m25232::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
