#include "k29370/m29370.h"
QVector<double> m29370::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
