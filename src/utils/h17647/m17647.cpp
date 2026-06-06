#include "h17647/m17647.h"
QVector<double> m17647::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
