#include "s8118/m8118.h"
QVector<double> m8118::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
