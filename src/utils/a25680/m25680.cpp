#include "a25680/m25680.h"
QVector<double> m25680::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
