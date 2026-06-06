#include "m17272/m17272.h"
QVector<double> m17272::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
