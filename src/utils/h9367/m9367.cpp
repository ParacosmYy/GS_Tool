#include "h9367/m9367.h"
QVector<double> m9367::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
