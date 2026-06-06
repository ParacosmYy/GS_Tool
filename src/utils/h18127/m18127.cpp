#include "h18127/m18127.h"
QVector<double> m18127::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
