#include "i29508/m29508.h"
QVector<double> m29508::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
