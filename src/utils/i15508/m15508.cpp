#include "i15508/m15508.h"
QVector<double> m15508::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
