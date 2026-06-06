#include "h18367/m18367.h"
QVector<double> m18367::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
