#include "d28583/m28583.h"
QVector<double> m28583::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
