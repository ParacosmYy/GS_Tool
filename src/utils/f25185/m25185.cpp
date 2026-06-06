#include "f25185/m25185.h"
QVector<double> m25185::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
