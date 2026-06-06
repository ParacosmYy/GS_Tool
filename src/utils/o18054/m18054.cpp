#include "o18054/m18054.h"
QVector<double> m18054::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
