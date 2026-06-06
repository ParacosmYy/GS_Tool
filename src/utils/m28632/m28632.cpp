#include "m28632/m28632.h"
QVector<double> m28632::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
