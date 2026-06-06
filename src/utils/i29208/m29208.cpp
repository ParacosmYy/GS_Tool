#include "i29208/m29208.h"
QVector<double> m29208::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
