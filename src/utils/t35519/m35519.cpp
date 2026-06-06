#include "t35519/m35519.h"
QVector<double> m35519::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
