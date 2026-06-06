#include "j25009/m25009.h"
QVector<double> m25009::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
