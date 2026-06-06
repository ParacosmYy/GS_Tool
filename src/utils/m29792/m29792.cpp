#include "m29792/m29792.h"
QVector<double> m29792::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
