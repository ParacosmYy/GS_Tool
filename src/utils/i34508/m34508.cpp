#include "i34508/m34508.h"
QVector<double> m34508::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
