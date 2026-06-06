#include "f25325/m25325.h"
QVector<double> m25325::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
