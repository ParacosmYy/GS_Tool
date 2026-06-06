#include "f9185/m9185.h"
QVector<double> m9185::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
