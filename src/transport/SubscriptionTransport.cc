#include "transport/ConnectionManager.hh"
#include "transport/SubscriptionTransport.hh"

using namespace gazebo;
using namespace transport;

////////////////////////////////////////////////////////////////////////////////
// Constructor
SubscriptionTransport::SubscriptionTransport()
{
}

////////////////////////////////////////////////////////////////////////////////
// Destructor
SubscriptionTransport::~SubscriptionTransport()
{
  gzdbg << "SubTransport::Destructor  msgtyp[" << this->GetMsgType() << "]\n";
  ConnectionManager::Instance()->RemoveConnection(this->connection);
}

////////////////////////////////////////////////////////////////////////////////
/// Initialize the publication link 
void SubscriptionTransport::Init( const ConnectionPtr &conn )
{
  this->connection = conn;
}

////////////////////////////////////////////////////////////////////////////////
/// Get the typename of the message that is handled
std::string SubscriptionTransport::GetMsgType() const
{
  return "";
}

////////////////////////////////////////////////////////////////////////////////
bool SubscriptionTransport::HandleMessage(const google::protobuf::Message *msg_)
{
  bool result = false;
  std::string data;
  msg_->SerializeToString(&data);

  if (this->connection->IsOpen())
  {
    this->connection->EnqueueMsg( data );
    result = true;
  }
  else
    gzdbg << "Connection closed[" << this->GetMsgType() << "]\n";

  return result;
}

////////////////////////////////////////////////////////////////////////////////
/// Output a message to a connection
bool SubscriptionTransport::HandleData(const std::string &newdata)
{
  bool result = false;
  std::string data;
  if (this->connection->IsOpen())
  {
    this->connection->EnqueueMsg(newdata);
    result = true;
  }
  else
  {
    gzdbg << "Connection closed[" << this->GetMsgType() << "]\n";
    this->connection.reset();
  }

  return result;
}


////////////////////////////////////////////////////////////////////////////////
/// Get the connection
const ConnectionPtr &SubscriptionTransport::GetConnection() const
{
  return this->connection;
}

////////////////////////////////////////////////////////////////////////////////
/// Return true if the callback is local, false if the callback is tied to a 
/// remote connection
bool SubscriptionTransport::IsLocal() const
{
  return false;
}


