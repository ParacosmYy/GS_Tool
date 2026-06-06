#include "h9807/m9807.h"
QVector<double> m9807::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
