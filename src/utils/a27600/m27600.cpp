#include "a27600/m27600.h"
QVector<double> m27600::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
