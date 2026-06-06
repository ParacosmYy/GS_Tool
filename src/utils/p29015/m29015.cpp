#include "p29015/m29015.h"
QVector<double> m29015::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
