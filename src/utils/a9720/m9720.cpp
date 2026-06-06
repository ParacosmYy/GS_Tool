#include "a9720/m9720.h"
QVector<double> m9720::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
