#include "h18707/m18707.h"
QVector<double> m18707::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
