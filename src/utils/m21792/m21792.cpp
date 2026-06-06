#include "m21792/m21792.h"
QVector<double> m21792::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
