#include "pch.h"



CLinearMover::CLinearMover(std::initializer_list<cv::Point2f> points, float duration)
    : CLinearMover(points.begin(), points.end(), duration)
{
}

cv::Point2f CLinearMover::update(float delta, float fCorrectionX/*=0.0f*/, float fCorrectionY/*=0.0f*/)
{
	static POINT* correction = &CDataCollection::GetInstance()->m_CorrectSpotPosition;
	
	const auto comparison = [](float left, const PointData& right) {
        return left < right.total;
    };
    
    m_elapsed = std::min(m_elapsed + delta, m_duration);
	
  
    const float length = (static_cast<float>(m_elapsed) / static_cast<float>(m_duration)) * m_length;
    auto& data = *std::prev(std::upper_bound(m_distances.begin(), m_distances.end(), length, comparison));

   
    const float percent = (length - data.total) / data.distance;
    cv::Point2f point;

	point.x = ((data.p1.x - fCorrectionX) + percent * ((data.p2.x - fCorrectionX) - (data.p1.x - fCorrectionX)));
    point.y = ((data.p1.y - fCorrectionY) + percent * ((data.p2.y - fCorrectionY) - (data.p1.y - fCorrectionY)));

   /* point.x = (data.p1.x + percent * (data.p2.x - data.p1.x));
	point.y = (data.p1.y + percent * (data.p2.y - data.p1.y));*/
    return point;
}

bool CLinearMover::isComplete() const
{
    return m_duration == m_elapsed;
}

void CLinearMover::setDuration(int duration)
{
    m_duration = duration;
}

int CLinearMover::getDuration() const
{
    return m_duration;
}

int CLinearMover::getElapsed() const
{
    return  m_elapsed;
}

void CLinearMover::updateLength()
{
	auto distance = [](float x1, float y1, float x2, float y2) -> float {
		return sqrt(((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)));
	};
	float length = 0.f;
	for (std::size_t index = 0; (index + 1) < m_points.size(); ++index) {
		const float dist = distance(m_points[index].x, m_points[index].y, m_points[index + 1].x, m_points[index + 1].y);
		m_distances.emplace_back(length, dist, m_points[index], m_points[index + 1]);
		length += dist;
	}
	m_length = length;
}