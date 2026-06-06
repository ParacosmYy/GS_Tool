#include "h9707/m9707.h"
QVector<double> m9707::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
